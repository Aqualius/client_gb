#include "expenses_part.h"

const QMap<QString, QString> ExpensesPart::names = { { "CURRENT", "Текущие" },
                                                     { "CAPITAL", "Капитальные" },
                                                     { "CREDIT_ISSUE", "Предоставление кредитов" },
                                                     { "CREDIT_REFUND", "Возврат кредитов" },
                                                     { "RESERVE_FUND", "Резервный фонд" },
                                                     { "ROAD_FUND", "Дорожный фонд" },
                                                     { "COAL_INDUSTRY_FUND", "Угольный фонд" },
                                                     { "INDUSTRY_FUND", "Промышленный фонд" } };
